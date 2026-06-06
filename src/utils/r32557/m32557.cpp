#include "r32557/m32557.h"
QVector<double> m32557::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
