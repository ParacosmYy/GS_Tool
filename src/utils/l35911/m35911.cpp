#include "l35911/m35911.h"
QVector<double> m35911::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
