#include "f28225/m28225.h"
QVector<double> m28225::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
