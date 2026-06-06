#include "b28581/m28581.h"
QVector<double> m28581::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
