#include "nitrishapewidget.h"

#include "obj/BSShaderTextureSet.h"
#include "obj/BSLightingShaderProperty.h"
#include "obj/NiProperty.h"

NiTriShapeWidget::NiTriShapeWidget(Niflib::NiTriShapeRef triShape, QListWidget *parent) : QListWidgetItem(parent)
{
    m_triShape = triShape;

    setText(m_triShape->GetName().c_str());
    setStatusTip(m_triShape->GetName().c_str());
    auto properties = m_triShape->GetRefs();
    for(auto prop : properties) {
        auto shader = Niflib::DynamicCast<Niflib::BSLightingShaderProperty>(prop);
        if(shader) {
            auto textureSet = shader->GetTextureSet();
            if(textureSet) {
                setStatusTip(textureSet->GetTexture(0).c_str());
            }
        }
    }
}

NiTriShapeWidget::~NiTriShapeWidget()
{

}
