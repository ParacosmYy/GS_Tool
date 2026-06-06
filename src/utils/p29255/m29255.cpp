#include "p29255/m29255.h"
QVector<double> m29255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
