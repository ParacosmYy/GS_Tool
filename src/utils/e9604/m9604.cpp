#include "e9604/m9604.h"
QVector<double> m9604::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
