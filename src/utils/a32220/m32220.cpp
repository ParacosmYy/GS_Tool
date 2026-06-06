#include "a32220/m32220.h"
QVector<double> m32220::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
