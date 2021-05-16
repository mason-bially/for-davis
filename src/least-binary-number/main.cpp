
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

        for (size_t i = a.size(); i > 0 ; --i)
        {
            if (a[i - 1] != b[i - 1])
            {
                return b[i - 1]; // if a is false then it is smaller
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
    // structs
    private:

        struct PerNumber;
        struct Sequence;
        struct Action;
        struct State;

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

        struct Sequence
        {
            std::string name;
            bitvec bits;
            std::set<TNum> containsNumbers;
        };

        struct Action
        {
            std::shared_ptr<Sequence> leftSeq;
            std::shared_ptr<Sequence> rightSeq;
            int64_t overlapAmount;

            int64_t scoreValue = -1;
            bitvec scoredBits;
            std::vector<std::shared_ptr<Sequence>> scoredSubsumes;

            void score(State const& s)
            {
                scoredBits = overlap(leftSeq->bits, rightSeq->bits, overlapAmount);
                spdlog::trace("overlap {} + {} ({}) -> {}", leftSeq->bits, rightSeq->bits, overlapAmount, scoredBits);

                // check for additional numbers removed by these overlaps
                scoreValue = overlapAmount;
                for (auto const& seq : s.sequences)
                    if (
                        // removing these checks appears to improve scores, why?
                        //seq != leftSeq && seq != rightSeq
                        //&&
                        contains(scoredBits, seq->bits) != -1
                        )
                    {
                        scoredSubsumes.push_back(seq);
                        scoreValue += seq->bits.size();
                    }
            }
        };

        struct State
        {
            std::vector<std::shared_ptr<Sequence>> sequences;
        };

        // state
    private:
        TNum _target = 1;

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

            State state;
            for (TNum i = _target; i >= maxUncovered; --i)
            {
                if (_table[i].covered)
                    continue;

                Sequence seq;
                seq.name = std::to_string(i);
                seq.bits = _table[i].numberBits;
                seq.containsNumbers = { i };
                state.sequences.push_back(std::make_shared<Sequence>(seq));
            }

            size_t accumulated_score = 0;
            while (state.sequences.size() > 1)
            {
                std::vector<Action> actions;
                // find possible actions
                for (size_t i = 0; i < state.sequences.size(); ++i)
                {
                    for (size_t j = i + 1; j < state.sequences.size(); ++j)
                    {
                        auto const& a = state.sequences[i];
                        auto const& b = state.sequences[j];
                        auto overlapsLeft = overlapCounts(a->bits, b->bits);
                        auto overlapsRight = overlapCounts(b->bits, a->bits);
                        if (!overlapsLeft.empty() || !overlapsRight.empty())
                            spdlog::trace("overlaps for ({}, {}) [{} | {}]", a->name, b->name, overlapsLeft, overlapsRight);
                        overlapsLeft.push_back(0);
                        overlapsRight.push_back(0);

                        for (auto left : overlapsLeft)
                        {
                            actions.push_back({
                                .leftSeq = a,
                                .rightSeq = b,
                                .overlapAmount = left,
                            });
                        }
                        for (auto right : overlapsRight)
                        {
                            actions.push_back({
                                .leftSeq = b,
                                .rightSeq = a,
                                .overlapAmount = right,
                            });
                        }
                    }
                }

                // score actions
                for (Action& action : actions)
                {
                    action.score(state);
                }

                // choose best action
                size_t bestAction = 0;
                int64_t bestScore = -1;
                for (size_t i = 0; i < actions.size(); ++i)
                {
                    spdlog::trace("score of {} is {}, v {} at {}", i, actions[i].scoreValue, toNumber(actions[i].scoredBits), actions[i].scoredBits.size());
                    // TODO redo this as a series of stable sorts?
                    // Some way of choosing priority anyway
                    if (actions[i].scoreValue > bestScore
                        // improves greedy?
                        || (actions[i].scoreValue == bestScore && (
                            // choose the smallest number first
                            smallerNumber(actions[i].scoredBits, actions[bestAction].scoredBits)
                            // choose smallest changes first
                            || actions[i].scoredBits.size() < actions[bestAction].scoredBits.size()
                        )))
                    {
                        spdlog::trace("is better!");
                        bestAction = i;
                        bestScore = actions[i].scoreValue;
                    }
                }

                // take action
                Action action = actions[bestAction];
                assert(action.scoreValue == bestScore);
                accumulated_score += action.scoreValue;

                spdlog::debug("SeqAndSeq ({1} + {2}) scored: {0}", action.scoreValue, action.leftSeq->name, action.rightSeq->name);
                std::set<TNum> scored_numbers = action.leftSeq->containsNumbers;
                scored_numbers.insert(action.rightSeq->containsNumbers.begin(), action.rightSeq->containsNumbers.end());
                for (auto s : action.scoredSubsumes)
                    scored_numbers.insert(s->containsNumbers.begin(), s->containsNumbers.end());

                // build new partial
                state.sequences.push_back(std::make_shared<Sequence>(Sequence {
                    .bits = action.scoredBits,
                    .containsNumbers = scored_numbers
                }));
                spdlog::trace("made seq {}, {}, {}", state.sequences.back()->name, state.sequences.back()->bits, state.sequences.back()->containsNumbers);

                std::erase_if(state.sequences, [&](auto s) {
                    return s == action.leftSeq || s == action.rightSeq
                    || std::find(action.scoredSubsumes.begin(), action.scoredSubsumes.end(), s) != action.scoredSubsumes.end();
                });

                spdlog::trace("loop remainingPartials count is {}", state.sequences.size());
            }

            // final result is 0th partial
            spdlog::info("a({}) is {} - {}   (acc_score: {})", _target, toNumber(state.sequences[0]->bits), state.sequences[0]->bits, accumulated_score);
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


