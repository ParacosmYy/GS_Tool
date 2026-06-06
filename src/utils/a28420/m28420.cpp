#include "a28420/m28420.h"
QVector<double> m28420::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
