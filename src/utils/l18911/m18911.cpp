#include "l18911/m18911.h"
QVector<double> m18911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
