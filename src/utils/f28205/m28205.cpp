#include "f28205/m28205.h"
QVector<double> m28205::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
