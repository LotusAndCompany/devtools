#include "random_data.h"

#include <QDateTime>

using namespace Test;

const QString RandomData::numbers = "1234567890";
const QString RandomData::lowerAlphabets = "qwertyuiopasdfghjklzxcvbnm";
const QString RandomData::upperAlphabets = "QWERTYUIOPASDFGHJKLZXCVBNM";
const QString RandomData::whitespaces = " \t\n";
// WARNING: %は書式設定文字列として使うので問題が発生する可能性がある
const QString RandomData::symbols = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
// NOTE: 改行・タブはあまり使わないだろうという想定
const QString RandomData::defaultCharsets = numbers + lowerAlphabets + upperAlphabets + " "
                                            + symbols;
const QString RandomData::ascii = numbers + lowerAlphabets + upperAlphabets + whitespaces + symbols;

RandomData::RandomData()
    : RandomData(QDateTime::currentMSecsSinceEpoch())
{}

RandomData::RandomData(int seed)
    : gen(seed)
{}

void RandomData::setSeed(int seed)
{
    gen.seed(seed);
}

int RandomData::nextInt()
{
    return gen.generate64();
}

int RandomData::nextInt(int max)
{
    return gen.bounded(max);
}

int RandomData::nextInt(int min, int max)
{
    return gen.bounded(min, max);
}

double RandomData::nextDouble()
{
    return gen.generateDouble();
}

double RandomData::nextDouble(double max)
{
    return gen.bounded(max);
}

double RandomData::nextDouble(double min, double max)
{
    return min + gen.bounded(max - min);
}

QString RandomData::nextQString()
{
    return nextQString(defaultQStringLength, defaultCharsets);
}

QString RandomData::nextQString(int length)
{
    return nextQString(length, defaultCharsets);
}

QString RandomData::nextQString(const QString &charSets)
{
    return nextQString(defaultQStringLength, charSets);
}

QString RandomData::nextQString(int length, const QString &charSets)
{
    QString result;
    for (int i = 0; i < length; i++)
        result.append(charSets.at(gen.bounded(charSets.length())));

    return result;
}
