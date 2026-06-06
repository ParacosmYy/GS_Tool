#include "a17880/m17880.h"
QVector<double> m17880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
