#include "b30381/m30381.h"
QVector<double> m30381::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
