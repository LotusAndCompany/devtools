#ifndef COMMON_EXCEPTION_H
#define COMMON_EXCEPTION_H

/**
 * @file
 * @brief 基本的な機能を持つ例外クラスと出力演算子のオーバーロードを定義
 */

#include <QDebug>
#include <QException>

#include <utility>

#ifdef _TEST_CommonException
namespace Test {
class TestCommonException;
}
#endif

/**
 * @brief メッセージを設定できる基本的な例外クラス
 * @details
 * 基底クラスのQExceptionは[std::exception](https://cpprefjp.github.io/reference/exception/exception.html)を継承していて、QObjectは継承していない
 *
 * @sa [QException](https://doc.qt.io/qt-6/qexception.html)
 */
class CommonException : public QException
{
public:
    /**
     * @brief デフォルトコンストラクタ
     */
    CommonException() = default;
    /**
     * @brief コピーコンストラクタ
     * @param src コピー元インスタンス
     */
    CommonException(const CommonException &src) = default;
    CommonException(CommonException &&src) noexcept = default;
    CommonException &operator=(const CommonException &src) = default;
    CommonException &operator=(CommonException &&src) noexcept = default;
    ~CommonException() override = default;
    /**
     * @brief 任意のメッセージを設定できるコンストラクタ
     * @param message メッセージ
     */
    explicit CommonException(QString message) : message(std::move(message)) {}

    /// メッセージ
    QString message;

    /**
     * @brief このインスタンスを投げる
     *
     * @sa [QException::raise()](https://doc.qt.io/qt-6/qexception.html#raise)
     */
    void raise() const override { throw *this; }

    // const char *what() const noexcept override { return message.toStdString().c_str(); }

protected:
    /**
     * @brief このインスタンスをコピーする
     * @return コピーされたインスタンス
     * @note この関数で生成されたインスタンスは`delete`等、何らかの方法で解放する必要がある
     *
     * @sa [QException::clone()](https://doc.qt.io/qt-6/qexception.html#clone)
     */
    [[nodiscard]] QException *clone() const override { return new CommonException(*this); }

#ifdef _TEST_CommonException
    friend class Test::TestCommonException;
#endif
};

/**
 * @brief QDebugに出力する演算子オーバーロード
 * @param debug [QDebug](https://doc.qt.io/qt-6/qdebug.html)オブジェクト
 * @param exception 例外クラスインスタンス
 * @return 入力を処理させたQDebugオブジェクト
 *
 * @details 要するに、`<<`演算子を使ってQDebugにメッセージを出力できるようにしている
 * @code
 * // これは
 * qDebug() << a;
 *
 * // QDebug operator<<(QDebug, const CommonException&)という関数を呼び出すイメージ
 * operator<<(qDebug(), a);
 * @endcode
 * @code
 * // こう書くと
 * qDebug() << a << b << c;
 *
 * // 左から評価されて
 * ((qDebug() << a) << b) << c;
 *
 * // こんな感じになる
 * operator<<(operator<<(operator<<(qDebug(), a), b), c);
 * @endcode
 */
// qDebug(), qInfo(), qWarning()等が参照を返さないのでQDebugを受け渡す
inline QDebug operator<<(QDebug debug, const CommonException &exception)
{
    return debug << exception.message;
}

#endif // COMMON_EXCEPTION_H
