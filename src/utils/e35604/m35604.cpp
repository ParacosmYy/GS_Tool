#include "e35604/m35604.h"
QVector<double> m35604::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
