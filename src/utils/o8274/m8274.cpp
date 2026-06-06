#include "o8274/m8274.h"
QVector<double> m8274::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
