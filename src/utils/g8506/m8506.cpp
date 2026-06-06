#include "g8506/m8506.h"
QVector<double> m8506::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
