#include "k18010/m18010.h"
QVector<double> m18010::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
