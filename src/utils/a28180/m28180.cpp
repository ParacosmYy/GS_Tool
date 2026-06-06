#include "a28180/m28180.h"
QVector<double> m28180::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
