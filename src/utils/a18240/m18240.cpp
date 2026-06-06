#include "a18240/m18240.h"
QVector<double> m18240::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
