#include "animation.h"

animation_t * animation_create(
        animation_type_t type,
        int ticks_per_frame,
        int num_frames,
        ...)
{
    va_list ap;

    va_start(ap, num_frames);

    animation_t * a = animation_createv(
            type,
            ticks_per_frame,
            num_frames,
            ap);

    va_end(ap);

    return a;
}

animation_t * animation_createa(
        animation_type_t type,
        int ticks_per_frame,
        int num_frames,
        char * filenames[])
{
    assert(filenames);

    int i = -1;
    BITMAP ** frames = 0;
    BITMAP * (*load)(const char *, RGB * pal);

    switch(type)
    {
        case ANIMATION_TYPE_BMP:
            load = load_bitmap;
            break;
        case ANIMATION_TYPE_PNG:
            load = load_png;
            break;
        default:
            return 0;
    }

    animation_t * a = malloc(sizeof(animation_t));

    if(!a)
        return 0;

    frames = malloc(sizeof(BITMAP **) * num_frames);

    if(!frames)
        goto error;

    for(i=0; i<num_frames; i++)
    {
        char * filename = filenames[i];

        assert(filename);

        frames[i] = load(filename, 0);

        if(!frames[i])
        {
             i--;
             goto error;
        }
    }

    a->frames_ = frames;
    a->start_ticks_ = 0;
    a->num_frames_ = num_frames;
    a->ticks_per_frame_ = ticks_per_frame;
    a->type_ = type;

    return a;

error:
    for(; i>=0; i--)
        destroy_bitmap(frames[i]);

    free(frames);

    free(a);

    return 0;
}

animation_t * animation_createf(
        animation_type_t type,
        int ticks_per_frame,
        int num_frames,
        const char * filename_format)
{
    animation_t * a = 0;
    char ** filenames = 0;
    int i = -1;
    int maxlen = strlen(filename_format) * 2;

    assert(filename_format);

    filenames = malloc(sizeof(char *) * num_frames);

    if(!filenames)
        return 0;

    for(i=0; i<num_frames; i++)
    {
        char * filename;
        int len;

        filename = malloc(sizeof(char) * maxlen);

        if(!filename)
        {
            i--;
            goto cleanup;
        }

        len = snprintf(filename, 1024, filename_format, i + 1);

        filename[len] = 0;

        filenames[i] = filename;
    }

    i--;

    a = animation_createa(
            type,
            ticks_per_frame,
            num_frames,
            filenames);

cleanup:
    for(; i>=0; i--)
        free(filenames[i]);

    free(filenames);

    return a;
}

animation_t * animation_createv(
        animation_type_t type,
        int ticks_per_frame,
        int num_frames,
        va_list ap)
{
    int i;

    char ** filenames = malloc(sizeof(char *) * num_frames);

    if(!filenames)
        return 0;

    for(i=0; i<num_frames; i++)
        filenames[i] = va_arg(ap, char *);

    animation_t * a = animation_createa(
            type,
            ticks_per_frame,
            num_frames,
            filenames);

    free(filenames);

    return a;
}

void animation_destroy(animation_t ** p_a)
{
    int i;

    assert(p_a);

    animation_t * a = *p_a;

    if(a)
    {
        for(i=0; i<a->num_frames_; i++)
            destroy_bitmap(a->frames_[i]);

        free(a->frames_);
        free(a);
        *p_a = a;
    }
}

BITMAP * animation_begin(animation_t * a, int ticks)
{
    assert(a);

    a->start_ticks_ = ticks;

    return animation_frame(a, ticks);
}

BITMAP * animation_frame(animation_t * a, int ticks)
{
    assert(a);
    assert(ticks >= a->start_ticks_);

    int past = (ticks - a->start_ticks_) / a->ticks_per_frame_;
    int current_frame_ = past % (a->num_frames_ - 1);

    return a->frames_[current_frame_];
}

