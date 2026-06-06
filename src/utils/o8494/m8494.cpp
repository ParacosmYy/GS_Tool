#include "o8494/m8494.h"
QVector<double> m8494::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
