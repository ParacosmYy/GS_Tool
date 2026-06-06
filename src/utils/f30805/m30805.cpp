#include "f30805/m30805.h"
QVector<double> m30805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
