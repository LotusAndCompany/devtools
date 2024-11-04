#ifndef DATA_CONVERSION_H
#define DATA_CONVERSION_H

#include <QObject>
#include <QVariant>

#include "core/tool/tool.h"

#ifdef _TEST_DataConversion
namespace Test {
class TestDataConversion;
}
#endif

/**
 * @brief シリアライズされたデータの変換、フォーマットを行うツールのインターフェイス
 * @todo 出力形式によって細かくオプションを設定できるようにする
 */
class DataConversionInterface : public Tool
{
    Q_OBJECT

public:
    /// デストラクタ
    virtual ~DataConversionInterface() = default;

    /**
     * @brief 入力文字列を返す
     * @return 入力文字列
     */
    const QString &inputText() const { return _inputText; }
    /**
     * @brief 入力文字列を設定する
     * @param inputText 設定する文字列
     */
    virtual void setInputText(const QString &inputText) = 0;
    /**
     * @brief 出力文字列を返す
     * @return 出力文字列
     */
    const QString &outputText() const { return _outputText; }
    /**
     * @brief ファイルを読み込む
     * @param path 読み込むファイルパス
     * @return 成功なら `true`
     */
    virtual bool load(const QString &path) = 0;
    /**
     * @brief ファイルに保存する
     * @param path 保存するファイルパス
     * @param overwrite `true` なら上書き
     * @return 成功なら `true`
     */
    virtual bool save(const QString &path, bool overwrite = false) const = 0;

    /// 出力形式を定義した列挙体
    enum class Format {
        MIN,        ///< 最小値
        UNKNOWN,    ///< 不明/未指定
        JSON,       ///< JSON
        YAML_BLOCK, ///< YAML (block style)
        YAML_FLOW,  ///< YAML (flow style)
        TOML,  ///< TOML
        ERROR, ///< 不正なフォーマット
        MAX,   ///< 最大値
    };
    /**
     * @brief 出力形式を設定する
     * @param format 出力形式
     */
    virtual void setOutputFormat(Format format) = 0;
    /**
     * @brief 出力形式を返す
     * @return 出力形式
     */
    Format outputFormat() const { return _outputFormat; }

    /// 出力文字列のインデントを定義した列挙体
    enum class Indentation {
        MIN,      ///< 最小値
        SPACES_4, ///< 4スペースでインデント
        SPACES_2, ///< 2スペースでインデント
        TABS,     ///< タブでインデント
        MINIFIED, ///< 最小化
        MAX,      ///< 最大値
    };
    /**
     * @brief 出力時のインデントを設定する
     * @param indentation インデント
     */
    virtual void setIndentation(Indentation indentation) = 0;
    /**
     * @brief 出力時のインデントを返す
     * @return 出力時のインデント
     */
    Indentation indentation() const { return _indentation; }

    /// 現在の設定で出力文字列を更新する
    virtual void updateOutputText() = 0;

protected:
    /**
     * @brief コンストラクタ
     * @param parent 親オブジェクト
     */
    explicit DataConversionInterface(QObject *parent = nullptr);

    /// 入力文字列
    QString _inputText;
    /// 出力文字列
    QString _outputText;
    /// 出力文字列の形式
    Format _outputFormat = Format::JSON;
    /// 出力文字列のインデント
    Indentation _indentation = Indentation::SPACES_4;

    /**
     * @brief 出力形式が不正なら例外を投げる
     * @param format 出力形式
     * @exception InvalidArgumentException &lt;int&gt; 不正な出力形式が渡された場合
     */
    static void validateFormat(Format format) noexcept(false);
    /**
     * @brief インデントが不正なら例外を投げる
     * @param indentation インデント
     * @exception InvalidArgumentException &lt;int&gt; 不正なインデントが渡された場合
     */
    static void validateIndentation(Indentation indentation) noexcept(false);
};

namespace YAML {
class Node;
};

/**
 * @brief シリアライズされたデータの変換、フォーマットを行うツール
 */
class DataConversion : public DataConversionInterface
{
    Q_OBJECT

public:
    /**
     * @brief コンストラクタ
     * @param parent 親オブジェクト
     */
    explicit DataConversion(QObject *parent = nullptr);
    /// デストラクタ
    ~DataConversion() = default;

    void setInputText(const QString &inputText) override;
    bool load(const QString &path) override;
    bool save(const QString &path, bool overwrite = false) const override;
    void setOutputFormat(Format format) override;
    void setIndentation(Indentation indentation) override;
    void updateOutputText() override;

private:
    /// ポインタが不正
    static const QString invalidPointer;

    /// _outputTextの更新が必要なら `true`
    bool outdated = false;

    /// 入力文字列の形式
    Format inputFormat = Format::UNKNOWN;
    /// 中間データ
    QVariant intermediateData;

    /// 入力文字列を解析し、 `inputFormat`, `intermediateData` を設定する
    void parseInputText();

#ifdef _TEST_DataConversion
    friend class Test::TestDataConversion;
#endif
};

#endif // DATA_CONVERSION_H
