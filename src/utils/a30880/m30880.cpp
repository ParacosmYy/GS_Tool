#include "a30880/m30880.h"
QVector<double> m30880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
