#include "basic_image_edit_interface.h"

bool BasicImageEditInterface::load(const QString &path)
{
    const bool result = loadImpl(path);

    outdated = true;
    return result;
}

void BasicImageEditInterface::reset()
{
    resetImpl();
    updateResult = updateImpl();

    outdated = false;
}

bool BasicImageEditInterface::update()
{
    if (outdated) {
        updateResult = updateImpl();
        outdated = false;
    }

    return updateResult;
}
