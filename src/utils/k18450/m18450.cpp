#include "k18450/m18450.h"
QVector<double> m18450::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
