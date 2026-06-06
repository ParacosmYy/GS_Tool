#include "g8686/m8686.h"
QVector<double> m8686::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
