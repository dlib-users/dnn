#ifndef DenseNet_H
#define DenseNet_H

#include <dlib/dnn.h>

namespace densenet
{
    // clang-format off
    using namespace dlib;
    // ACT can be any activation layer, BN must be bn_con or affine layer and k is the growth rate
    template <template <typename> class ACT, template <typename> class BN, long k>
    struct def
    {
        template <long num_filters, long ks, int s, typename SUBNET>
        using conp = add_layer<con_<num_filters, ks, ks, s, s, ks/2, ks/2>, SUBNET>;

        template <typename INPUT>
        using stem = add_layer<max_pool_<3, 3, 2, 2, 1, 1>, ACT<BN<conp<2 * k, 7, 2, INPUT>>>>;

        template <long num_filters, typename SUBNET>
        using transition = avg_pool<2, 2, 2, 2, con<num_filters, 1, 1, 1, 1, ACT<BN<SUBNET>>>>;

        template <long num_filters, typename SUBNET>
        using dense_layer = concat2<tag1, tag2,
                            tag2<conp<k, 3, 1,
                            ACT<BN<conp<4 * k, 1, 1,
                            ACT<BN<tag1<SUBNET>>>>>>>>>;

        template <typename SUBNET> using dense_layer_4k = dense_layer<4 * k  , SUBNET>;
        template <typename SUBNET> using dense_layer_8k = dense_layer<8 * k  , SUBNET>;
        template <typename SUBNET> using dense_layer_16k = dense_layer<16 * k, SUBNET>;
        template <typename SUBNET> using dense_layer_32k = dense_layer<32 * k, SUBNET>;

        template <size_t nb_32k, size_t nb_16k, size_t nb_8k, size_t nb_4k, typename INPUT>
        using backbone = ACT<BN<
                         repeat<nb_32k, dense_layer_32k, transition<k * (2 + nb_4k + 2 * nb_8k + 4 * nb_16k) / 8,
                         repeat<nb_16k, dense_layer_16k, transition<k * (2 + nb_4k + 2 * nb_8k) / 4,
                         repeat<nb_8k, dense_layer_8k, transition<k * (2 + nb_4k) / 2,
                         repeat<nb_4k, dense_layer_4k, stem<INPUT>>>>>>>>>>;
    };

    template <typename SUBNET>
    using classification_head = loss_multiclass_log<fc<1000, avg_pool_everything<SUBNET>>>;

    using train_121 = classification_head<def<relu, bn_con, 32>::backbone<16, 24, 12, 6, input_rgb_image>>;
    using infer_121 = classification_head<def<relu, affine, 32>::backbone<16, 24, 12, 6, input_rgb_image>>;
    using train_169 = classification_head<def<relu, bn_con, 32>::backbone<32, 32, 12, 6, input_rgb_image>>;
    using infer_169 = classification_head<def<relu, affine, 32>::backbone<32, 32, 12, 6, input_rgb_image>>;
    using train_201 = classification_head<def<relu, bn_con, 32>::backbone<32, 48, 12, 6, input_rgb_image>>;
    using infer_201 = classification_head<def<relu, affine, 32>::backbone<32, 48, 12, 6, input_rgb_image>>;
    using train_264 = classification_head<def<relu, bn_con, 32>::backbone<48, 64, 12, 6, input_rgb_image>>;
    using infer_264 = classification_head<def<relu, affine, 32>::backbone<48, 64, 12, 6, input_rgb_image>>;
    using train_161 = classification_head<def<relu, bn_con, 48>::backbone<24, 36, 12, 6, input_rgb_image>>;
    using infer_161 = classification_head<def<relu, affine, 48>::backbone<24, 36, 12, 6, input_rgb_image>>;

    // clang-format on
}  // namespace densenet

#endif  // DenseNet_H
