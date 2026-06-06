#include "b8341/m8341.h"
QVector<double> m8341::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
