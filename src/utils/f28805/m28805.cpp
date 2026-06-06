#include "f28805/m28805.h"
QVector<double> m28805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
