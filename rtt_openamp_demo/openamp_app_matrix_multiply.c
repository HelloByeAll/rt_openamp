#include "openamp_app.h"
#include "rtdbg.h"

#ifdef AMP_DEMO_MATRIX
#define     MAX_SIZE      6
#define     NUM_MATRIX    2

typedef struct _matrix
{
    rt_uint32_t size;
    rt_uint32_t elements[MAX_SIZE][MAX_SIZE];
} matrix;

#ifdef RT_AMP_MASTER
static struct _matrix i_matrix[2];
static struct _matrix e_matrix;
#endif /* RT_AMP_MASTER */

static struct rpmsg_endpoint* app_ept;

static void matrix_multiply(const matrix* m, const matrix* n, matrix* r)
{
    rt_uint32_t i, j, k;

    rt_memset(r, 0x0, sizeof(matrix));
    r->size = m->size;

    for (i = 0; i < m->size; ++i)
    {
        for (j = 0; j < n->size; ++j)
        {
            for (k = 0; k < r->size; ++k)
            {
                r->elements[i][j] += m->elements[i][k] * n->elements[k][j];
            }
        }
    }
}

static void matrix_print(struct _matrix* m)
{
    rt_uint32_t i, j;

    /* Generate two random matrices */
    LOG_I("Printing matrix... ");

    for (i = 0; i < m->size; ++i)
    {
        for (j = 0; j < m->size; ++j)
        {
            rt_kprintf(" %u ", m->elements[i][j]);
        }
        rt_kprintf("\r\n");
    }
}

static int rpmsg_endpoint_cb(struct rpmsg_endpoint* ept, void* data,
    size_t len, uint32_t src, void* priv)
{
#ifdef RT_AMP_MASTER
    struct _matrix* r_matrix = (struct _matrix*)data;
    int i, j, err_cnt = 0;

    (void)ept;
    (void)priv;
    (void)src;
    if (len != sizeof(struct _matrix))
    {
        LOG_E("Received matrix is of invalid len: %d:%lu",
            (int)sizeof(struct _matrix), (unsigned long)len);
        return RPMSG_SUCCESS;
    }

    for (i = 0; i < MAX_SIZE; i++)
    {
        for (j = 0; j < MAX_SIZE; j++)
        {
            if (r_matrix->elements[i][j] != e_matrix.elements[i][j])
            {
                err_cnt++;
                break;
            }
        }
    }

    if (err_cnt)
    {
        LOG_E("Result mismatched...");
        LOG_E("Expected matrix:");
        matrix_print(&e_matrix);
        LOG_E("Actual matrix:\r\n");
        matrix_print(r_matrix);
    }

#else
    matrix matrix_array[NUM_MATRIX];
    matrix matrix_result;

    (void)priv;
    (void)src;

    rt_memcpy(matrix_array, data, len);
    /* Process received data and multiple matrices. */
    matrix_multiply(&matrix_array[0], &matrix_array[1], &matrix_result);

    /* Send the result of matrix multiplication back to host. */
    rpmsg_send(ept, &matrix_result, sizeof(matrix));
#endif /* RT_AMP_MASTER */

    return 0;
}

void openamp_app_matrix_multiply_init(rt_openamp_t* app, struct rpmsg_endpoint* ept,
    const char* name, rt_uint32_t src, rt_uint32_t dest)
{
    rt_uint32_t status;

    app_ept = ept;
    status = openamp_app_create_endpoint(ept, &app->rvdev.rdev, name, src, dest, rpmsg_endpoint_cb, RT_NULL);
    if (status)
    {
        LOG_E("openamp_app_create_endpoint = 0x%X", status);
    }
}

#ifdef RT_AMP_MASTER
static void generate_matrices(rt_int32_t num_matrices,
    rt_uint32_t matrix_size, void* p_data)
{
    rt_uint32_t i, j, k;
    struct _matrix* p_matrix = p_data;
    rt_uint64_t value;

    for (i = 0; i < (rt_uint32_t)num_matrices; i++)
    {
        /* Initialize workload */
        p_matrix[i].size = matrix_size;

        LOG_I("Input matrix %d", i);
        for (j = 0; j < matrix_size; j++)
        {
            for (k = 0; k < matrix_size; k++)
            {

                value = (rand() & 0x7F);
                value = value % 10;
                p_matrix[i].elements[j][k] = value;
                rt_kprintf(" %u ", p_matrix[i].elements[j][k]);
            }
            rt_kprintf("\r\n");
        }
    }
}

static void app_matrix_multiply_test(void)
{
    srand(rt_tick_get());

    generate_matrices(2, MAX_SIZE, i_matrix);
    matrix_multiply(&i_matrix[0], &i_matrix[1], &e_matrix);
    rpmsg_send(app_ept, i_matrix, sizeof(i_matrix));
}
MSH_CMD_EXPORT(app_matrix_multiply_test, app_matrix_multiply_test);
#endif
#endif /* AMP_DEMO_MATRIX */