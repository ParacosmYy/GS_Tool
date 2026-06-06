#include "c8922/m8922.h"
QVector<double> m8922::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
