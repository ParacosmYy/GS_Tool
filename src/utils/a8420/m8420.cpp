#include "a8420/m8420.h"
QVector<double> m8420::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
