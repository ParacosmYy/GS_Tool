#include "e7884/m7884.h"
QVector<double> m7884::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
