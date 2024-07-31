#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <QString>
#include <QRandomGenerator>

namespace Test
{
class RandomData
{
    QRandomGenerator gen;
public:
    explicit RandomData() {}
    explicit RandomData(int seed);

    static const QString numbers;
    static const QString lowerAlphabets;
    static const QString upperAlphabets;
    static const QString symbols;
    static const QString whitespaces;
    static const QString ascii;
    static const QString defaultCharsets;
    static const int defaultQStringLength = 8;

    void setSeed(int seed);

    int nextInt();
    int nextInt(int max);
    int nextInt(int min, int max);

    double nextDouble();
    double nextDouble(double max);
    double nextDouble(double min, double max);

    QString nextQString();
    QString nextQString(int length);
    QString nextQString(const QString& charSets);
    QString nextQString(int length, const QString& charSets);
};
} // namespace Test

#endif  // TEST_UTIL_H