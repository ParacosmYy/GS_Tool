#include "a8220/m8220.h"
QVector<double> m8220::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
