#include "g35506/m35506.h"
QVector<double> m35506::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
