#include "o7914/m7914.h"
QVector<double> m7914::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
