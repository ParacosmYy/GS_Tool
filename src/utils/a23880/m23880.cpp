#include "a23880/m23880.h"
QVector<double> m23880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
