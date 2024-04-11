#ifndef ANALYZER_H
#define ANALYZER_H

class Analyzer  // NOLINT
{
    public:
        Analyzer();
        ~Analyzer();

        // Signal callbacks
        void load_left_video();
    private:
        int a;
};

#endif // ANALYZER_H