#include "h8807/m8807.h"
QVector<double> m8807::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
