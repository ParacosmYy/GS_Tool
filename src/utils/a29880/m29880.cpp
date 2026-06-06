#include "a29880/m29880.h"
QVector<double> m29880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
