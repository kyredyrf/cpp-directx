#pragma once

/**
 *  初期化、メイン処理、終了処理を順番に実行する。
 *  初期化に失敗したときはメイン処理は行わない。
 *
 *  @param onInit 初期化ファンクタ
 *  @param onMain メイン処理ファンクタ
 *  @param onExit 終了処理ファンクタ
 */
template <class _Ty1, class _Ty2, class _Ty3>
inline void SimpleWork(_Ty1 onInit, _Ty2 onMain, _Ty3 onExit)
{
    if (onInit())
    {
        onMain();
    }

    onExit();
}

/**
 *  初期化、メイン処理、終了処理を順番に実行する。
 *  初期化に失敗したときはメイン処理は行わない。
 *
 *  @return メイン処理の結果
 *  @param onInit 初期化ファンクタ
 *  @param onMain メイン処理ファンクタ
 *  @param onExit 終了処理ファンクタ
 */
template <class _ReturnType, class _Ty1, class _Ty2, class _Ty3>
inline _ReturnType SimpleWork(_Ty1 onInit, _Ty2 onMain, _Ty3 onExit)
{
    auto result = _ReturnType();

    if (onInit())
    {
        result = onMain();
    }

    onExit();

    return result;
}
