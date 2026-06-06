#include "i9088/m9088.h"
QVector<double> m9088::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
