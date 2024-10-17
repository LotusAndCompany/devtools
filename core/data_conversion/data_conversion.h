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
        /*XML,        ///< XML*/
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
        TAB,      ///< タブでインデント
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

    /// 現在の設定で出力文字列を更新する
    virtual void updateOutputText() = 0;

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

protected:
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

    /// 解析結果を格納する構造体
    struct ParseResult
    {
        Format format = Format::ERROR; ///< フォーマット
        QString error;                 ///< エラーメッセージ
        QVariant data;                 ///< データ
    };

    /**
     * @brief pがnullptrなら例外を投げる
     * @param p チェック対象
     */
    static void validatePointer(void *p) noexcept(false);

    /**
     * @brief JSONで解析できるか試す
     * @param result 結果の出力先
     */
    void tryParseJSON(ParseResult *result) const;

    /**
     * @brief YAMLで解析できるか試す
     * @param result 結果の出力先
     */
    void tryParseYAML(ParseResult *result) const;
    /**
     * @brief nodeを `QVariant` に変換する
     * @param src 変換前の値
     * @param dst 変換後の値の格納先
     */
    static bool yamlNodeToQVariant(const YAML::Node &src, QVariant *dst);
    /**
     * @brief scalarを `QVariant` に変換する
     * @details 基本的にはQStringを返す
     * @param src 変換前の値
     * @param dst 変換後の値の格納先
     */
    static bool yamlScalarToQVariant(const YAML::Node &src, QVariant *dst) noexcept(false);
    /**
     * @brief mapを `QVariantMap` に変換する
     * @param src 変換前の値
     * @param dst 変換後の値の格納先
     */
    static bool yamlMapToQVariantMap(const YAML::Node &src, QVariantMap *dst) noexcept(false);
    /**
     * @brief mapを `QVariantList` に変換する
     * @param src 変換前の値
     * @param dst 変換後の値の格納先
     */
    static bool yamlMapToQVariantList(const YAML::Node &src, QVariantList *dst) noexcept(false);

    /**
     * @brief TOMLで解析できるか試す
     * @param result 結果の出力先
     */
    void tryParseTOML(ParseResult *result) const;

#ifdef _TEST_DataConversion
    friend class Test::TestDataConversion;
#endif
};

#endif // DATA_CONVERSION_H
