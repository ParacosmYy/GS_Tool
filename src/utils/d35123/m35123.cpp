#include "d35123/m35123.h"
QVector<double> m35123::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
