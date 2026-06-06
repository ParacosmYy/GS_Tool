#include "c18722/m18722.h"
QVector<double> m18722::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
