#include "i28128/m28128.h"
QVector<double> m28128::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
