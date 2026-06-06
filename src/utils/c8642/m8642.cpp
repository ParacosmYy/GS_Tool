#include "c8642/m8642.h"
QVector<double> m8642::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
