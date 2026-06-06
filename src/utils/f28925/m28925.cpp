#include "f28925/m28925.h"
QVector<double> m28925::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
