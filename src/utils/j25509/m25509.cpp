#include "j25509/m25509.h"
QVector<double> m25509::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
