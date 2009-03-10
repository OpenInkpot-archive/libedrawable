#include "edrawable.h"

EDrawable *
e_drawable_new() {

};

void
e_drawable_init(EDrawable *e){
    Ewl_Image * i;
    
    i = EWL_IMAGE(e);
    ewl_image_init(i);

}
