#include "e8244/m8244.h"
QVector<double> m8244::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
