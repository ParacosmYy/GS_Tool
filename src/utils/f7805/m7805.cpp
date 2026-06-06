#include "f7805/m7805.h"
QVector<double> m7805::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
