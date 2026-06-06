#include "e28804/m28804.h"
QVector<double> m28804::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
