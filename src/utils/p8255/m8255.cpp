#include "p8255/m8255.h"
QVector<double> m8255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
