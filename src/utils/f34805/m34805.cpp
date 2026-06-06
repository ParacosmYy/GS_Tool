#include "f34805/m34805.h"
QVector<double> m34805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
