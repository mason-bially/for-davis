
#include "CLI/CLI.hpp"
#include "spdlog/spdlog.h"

#include "formatters.hpp"

namespace A056744
{
    typedef std::vector<bool> bitvec;

    uint64_t toNumber(bitvec const& bv)
    {
        return std::accumulate(
            bv.rbegin(), bv.rend(), 0ull,
            [](auto acc, auto bit) { return (acc << 1) | bit; });
    }

    bool smallerNumber(bitvec const& a, bitvec const& b)
    {
        if (a.size() < b.size())
            return true;
        else if (b.size() < a.size())
            return false;

        for (int i = 0; i < a.size(); ++i)
        {
            if (a[i] != b[i])
            {
                return b[i]; // if a is false then it is smaller
            }
        }

        return false; // equal
    }

    int64_t contains(bitvec const& base, bitvec const& target, int64_t base_start_index = 0)
    {
        int64_t bsize = base.size();
        int64_t tsize = target.size();
        if ((bsize + base_start_index) < tsize)
            return -1;

        int64_t end_search = (bsize - tsize) + 1;
        for (int64_t i = base_start_index; i < end_search; ++i)
        {
            int64_t end_check = i + tsize;
            for (int64_t j = i; j < end_check; ++j)
            {
                if (base[j] != target[j - i])
                    goto continue_search;
            }
            return i;
            continue_search: ;
        }
        return -1;
    }

    std::vector<int64_t> overlapCounts(bitvec const& left, bitvec const& right, int64_t maxOverlap = std::numeric_limits<int64_t>::max())
    {
        int64_t lsize = left.size();
        int64_t rsize = right.size();
        int64_t minsize = std::min(maxOverlap, std::min(lsize, rsize));

        std::vector<int64_t> results;
        for (int64_t i = 1; i <= minsize; ++i)
        {
            for (int64_t j = 0; j < i; ++j)
            {
                if (left[j] != right[(rsize-i)+j])
                    goto continue_search;
            }
            results.insert(results.begin(), i);
            continue_search: ;
        }

        return results;
    }

    bitvec overlap(bitvec const& left, bitvec const& right, int64_t count)
    {
        int64_t lsize = left.size();
        int64_t rsize = right.size();
        int64_t ressize = lsize + rsize - count;

        bitvec res(ressize);
        for (int64_t i = 0; i <= ressize; ++i)
        {
            if (i < rsize - count)
            {
                res[i] = right[i];
            }
            else if (i >= ressize - (lsize - count))
            {
                res[i] = left[i - (rsize - count)];
            }
            else
            {
                assert (right[i] == left[i - (rsize - count)]);
                res[i] = right[i];
            }
        }

        return res;
    }

    template<typename TNum = uint16_t>
    class Calculate0
    {
    private:
        TNum _target = 1;

        struct PerNumber {
            bool satisfied = false;
            bool covered = false;
            TNum number;
            bitvec numberBits;
            std::set<TNum> containsNumbers;

            void calculateContainedNumbers(Calculate0& calc)
            {
                containsNumbers.insert(number);

                for (TNum sn = number - 1; sn > 0; --sn)
                {
                    if (containsNumbers.contains(sn))
                        continue;
                    PerNumber& snt = calc._table[sn];
                    if (contains(numberBits, snt.numberBits) != -1)
                    {
                        containsNumbers.insert(snt.containsNumbers.begin(), snt.containsNumbers.end());
                        snt.covered = true;
                    }
                }
            }

            void calculateBitVec()
            {
                TNum filter = 1;
                while (filter <= number)
                {
                    numberBits.push_back(number & filter);
                    filter <<= 1;
                }

            }

            void init(Calculate0& calc, TNum num)
            {
                number = num;
                calculateBitVec();
                calculateContainedNumbers(calc);

                spdlog::trace("init table {}, {} ({})", number, numberBits, containsNumbers);
            }
        };
        friend struct PerNumber;
        std::vector<PerNumber> _table;

    public:
        Calculate0() = default;

    public:
        void setTarget(TNum n)
        {
            _target = n;
        }

        // TODO take in interrupt / mutex object
        void calculate()
        {
            _table.resize(_target + 1);
            for (TNum n = 1; n <= _target; ++n)
                _table[n].init(*this, n);

            TNum maxUncovered;
            for (maxUncovered = 1; _table[maxUncovered].covered; ++maxUncovered);

            spdlog::debug("maxUncovered is {}", maxUncovered);

            std::set<TNum> remainingNumbers;
            for (auto i = maxUncovered; i <= _target; ++i)
            {
                if (!_table[i].covered)
                    remainingNumbers.insert(i);
            }

            spdlog::debug("remainingNumbers are {}", remainingNumbers);

            struct PartialSequence
            {
                bitvec bits;
                std::set<TNum> containsNumbers;
            };

            struct Action
            {
                enum class Kind
                {
                    NumAndNum,
                    NumAndPartial,
                    PartialAndPartial,
                };
                Kind kind;
                TNum srcNum = 0;
                TNum srcNumBigger = 0;
                size_t dstPartial;
                size_t srcPartial;

                int64_t scoreValue = -1; // reset score to -1 to force rescore of action
                std::set<TNum> scoredNumbers;
                bitvec scoredBits;
                bool scoredAttachLeft; // attach srcNum or srcPartial to left.

                void score(Calculate0 const& calc, std::set<TNum> remaining_nums, bitvec const& a, bitvec const& b, std::set<TNum> scored_nums_base)
                {
                    // TODO branch? how deal with equal scored sides?
                    auto overlapsLeft = overlapCounts(a, b);
                    auto overlapsRight = overlapCounts(b, a);
                    spdlog::trace("overlaps [{}] [{}]", overlapsLeft, overlapsRight);

                    // score overlaps
                    scoreValue = 0;
                    scoredNumbers = scored_nums_base;
                    scoredBits = overlap(a, b, 0);
                    scoredAttachLeft = true;
                    for (auto left : overlapsLeft)
                    {
                        auto the_overlap = overlap(a, b, left);
                        spdlog::trace(" < overlap {} + {} ({}) -> {}", a, b, left, the_overlap);
                        // check for additional numbers removed by these overlaps
                        std::set<TNum> scored_numbers = scored_nums_base;
                        auto score = left;
                        for (auto n : remaining_nums)
                            if (!scored_numbers.contains(n) && contains(the_overlap, calc._table[n].numberBits) != -1)
                            {
                                scored_numbers.insert(n);
                                score += calc._table[n].numberBits.size();
                            }
                        if (score > scoreValue || (score == scoreValue && smallerNumber(the_overlap, scoredBits)))
                        {
                            scoreValue = score;
                            scoredNumbers = scored_numbers;
                            scoredBits = the_overlap;
                            scoredAttachLeft = true;
                        }
                    }
                    for (auto right : overlapsRight)
                    {
                        auto the_overlap = overlap(b, a, right);
                        spdlog::trace(" > overlap {} + {} ({}) -> {}", b, a, right, the_overlap);
                        // check for additional numbers removed by these overlaps
                        std::set<TNum> scored_numbers = scored_nums_base;
                        auto score = right;
                        for (auto n : remaining_nums)
                            if (!scored_numbers.contains(n) && contains(the_overlap, calc._table[n].numberBits) != -1)
                            {
                                scored_numbers.insert(n);
                                score += calc._table[n].numberBits.size();
                            }
                        if (score > scoreValue)
                        {
                            scoreValue = score;
                            scoredNumbers = scored_numbers;
                            scoredBits = the_overlap;
                            scoredAttachLeft = false;
                        }
                    }
                }
            };

            PartialSequence final;
            final.bits = _table[_target].numberBits;
            final.containsNumbers.insert(_target);
            remainingNumbers.erase(_target);
            std::vector<PartialSequence> partials { final };

            size_t accumulated_score = 0;
            std::vector<Action> actions;
            for (TNum i = maxUncovered; i < _target; ++i)
            {
                for (TNum j = i + 1; j < _target; ++j)
                {
                    Action action {
                        .kind = Action::Kind::NumAndNum,
                        .srcNum = i,
                        .srcNumBigger = j
                    };
                    actions.push_back(action);
                }
            }

            size_t lastModifiedPartial = 0;
            while (!remainingNumbers.empty() || partials.size() > 1)
            {
                // find new possible actions
                for (auto n : remainingNumbers)
                {
                    Action action {
                            .kind = Action::Kind::NumAndPartial,
                            .srcNum = n,
                            .dstPartial = lastModifiedPartial
                    };
                    actions.push_back(action); // TODO using only these actions makes some answers correct, aothers wrong, why?
                }
                for (size_t i = 0; i < partials.size(); ++i)
                {
                    if (lastModifiedPartial == i)
                        continue;

                    Action action {
                            .kind = Action::Kind::PartialAndPartial,
                            .dstPartial = std::min(lastModifiedPartial, i),
                            .srcPartial = std::max(lastModifiedPartial, i),
                    };
                    actions.push_back(action);
                }

                // rescore actions
                for (Action& action : actions)
                {
                    PartialSequence const& sp = partials[action.srcPartial];
                    PartialSequence const& dp = partials[action.dstPartial];
                    std::set<TNum> seq_set;

                    switch (action.kind)
                    {
                        // TODO check for partials in partial overlaps?
                        case Action::Kind::NumAndNum:
                            action.score(*this, remainingNumbers,
                                         _table[action.srcNum].numberBits, _table[action.srcNumBigger].numberBits,
                                         { action.srcNum, action.srcNumBigger });
                            break;
                        case Action::Kind::NumAndPartial:
                            seq_set = { action.srcNum };
                            seq_set.insert(dp.containsNumbers.begin(), dp.containsNumbers.end());
                            action.score(*this, remainingNumbers,
                                         _table[action.srcNum].numberBits, dp.bits,
                                         seq_set);
                            break;
                        case Action::Kind::PartialAndPartial:
                            seq_set = sp.containsNumbers;
                            seq_set.insert(dp.containsNumbers.begin(), dp.containsNumbers.end());
                            action.score(*this, remainingNumbers,
                                         sp.bits, dp.bits,
                                         seq_set);
                            break;
                    }
                }

                // choose best action
                size_t bestAction = 0;
                int64_t bestScore = -1;
                for (size_t i = 0; i < actions.size(); ++i)
                {
                    if (actions[i].scoreValue > bestScore)
                    {
                        bestAction = i;
                        bestScore = actions[i].scoreValue;
                    }
                }

                // take action
                Action action = actions[bestAction];
                assert(action.scoreValue == bestScore);
                accumulated_score += action.scoreValue;
                switch (action.kind)
                {
                    case Action::Kind::NumAndNum:
                    {
                        spdlog::debug("NumAndNum({1}, {2}) scored: {0}", action.scoreValue, action.srcNum, action.srcNumBigger);
                        // build new partial
                        PartialSequence new_partial {
                            .bits = action.scoredBits,
                            .containsNumbers = action.scoredNumbers
                        };
                        partials.push_back(new_partial);
                        lastModifiedPartial = partials.size() - 1;
                        spdlog::trace("partial {}, {} ({})", lastModifiedPartial, new_partial.bits, new_partial.containsNumbers);

                        // correct actions and remaining numbers
                        std::erase_if(actions,
                            [&](Action a) { return
                                action.scoredNumbers.contains(a.srcNum) || action.scoredNumbers.contains(a.srcNumBigger);
                            });
                        std::erase_if(remainingNumbers,
                            [&](TNum n) { return
                                action.scoredNumbers.contains(n);
                            });
                    }   break;

                    case Action::Kind::NumAndPartial:
                    {
                        spdlog::debug("NumAndPartial({1}, {2}) scored: {0}", action.scoreValue, action.srcNum, action.dstPartial);
                        // build new partial
                        partials[action.dstPartial].bits = action.scoredBits;
                        partials[action.dstPartial].containsNumbers = action.scoredNumbers;
                        lastModifiedPartial = action.dstPartial;
                        spdlog::trace("partial {}, {} ({})", action.dstPartial, partials[action.dstPartial].bits, partials[action.dstPartial].containsNumbers);

                        // correct actions and remaining numbers
                        std::erase_if(actions,
                            [&](Action a) { return
                                action.scoredNumbers.contains(a.srcNum) || action.scoredNumbers.contains(a.srcNumBigger)
                                || a.srcPartial == action.dstPartial || a.dstPartial == action.dstPartial;
                            });
                        std::erase_if(remainingNumbers,
                            [&](TNum n) { return
                                action.scoredNumbers.contains(n);
                            });
                    }   break;

                    case Action::Kind::PartialAndPartial:
                    {
                        spdlog::debug("PartialAndPartial({1}, {2}) scored: {0}", action.scoreValue, action.srcPartial, action.dstPartial);
                        // build new partial
                        partials[action.dstPartial].bits = action.scoredBits;
                        partials[action.dstPartial].containsNumbers = action.scoredNumbers;
                        lastModifiedPartial = action.dstPartial;
                        spdlog::trace("partial {}, {} ({})", action.dstPartial, partials[action.dstPartial].bits, partials[action.dstPartial].containsNumbers);
                        partials.erase(partials.begin() + action.srcPartial);

                        // correct actions and remaining numbers
                        std::erase_if(actions,
                            [&](Action a) { return
                                action.scoredNumbers.contains(a.srcNum) || action.scoredNumbers.contains(a.srcNumBigger)
                                || a.srcPartial == action.dstPartial || a.dstPartial == action.dstPartial
                                || a.srcPartial == action.srcPartial || a.dstPartial == action.srcPartial;
                            });
                        std::erase_if(remainingNumbers,
                            [&](TNum n) { return
                                action.scoredNumbers.contains(n);
                            });

                        // fixup action indexes.
                        for (Action& a : actions)
                        {
                            if (a.srcPartial > action.srcPartial)
                                a.srcPartial -= 1;
                            if (a.dstPartial > action.srcPartial)
                                a.dstPartial -= 1;
                        }
                    }   break;
                }

                spdlog::trace("loop remainingNumbers is {}, remainingPartials count is {}", remainingNumbers, partials.size());
            }

            // final result is 0th partial
            spdlog::info("a({}) is {} - {}   (acc_score: {})", _target, toNumber(partials[0].bits), partials[0].bits, accumulated_score);
        }
    };
}


int main(int argc, char** argv)
{
    CLI::App app{"Attempts to calculate A056744 efficiently."};

    uint16_t target = 1;
    app.add_option("target", target, "The number to calculate to.");

    uint16_t from = 0;
    app.add_option("--from", from, "The value to start calculating from.");

    bool debug = false;
    app.add_flag("--debug", debug, "Whether to display debug information or not.");
    bool trace = false;
    app.add_flag("--trace", trace, "Whether to display trace information or not (also --debug).");

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError &e) {
        return app.exit(e);
    }

    if (debug)
        spdlog::set_level(spdlog::level::debug);
    if (trace)
        spdlog::set_level(spdlog::level::trace);

    if (from == 0)
    {
        spdlog::info("target is {}", target);
        from = target;
    }
    else
        spdlog::info("calculating from {} to target {}", from, target);

    for (auto i = from; i <= target; ++i)
    {
        A056744::Calculate0 calculator;
        calculator.setTarget(i);
        calculator.calculate();
    }

    return 0;
}


