#include "m18452/m18452.h"
QVector<double> m18452::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
