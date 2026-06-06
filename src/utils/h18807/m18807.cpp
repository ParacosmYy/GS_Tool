#include "h18807/m18807.h"
QVector<double> m18807::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
