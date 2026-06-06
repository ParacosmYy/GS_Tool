#include "i28008/m28008.h"
QVector<double> m28008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
